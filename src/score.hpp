#pragma once



namespace azalea {

struct Score {
    int early = 0;
    int late  = 0;

    constexpr Score operator+(const Score& rhs) const noexcept {
        return Score{early+rhs.early, late+rhs.late};
    }
    constexpr Score operator-(const Score& rhs) const noexcept {
        return Score{early-rhs.early, late-rhs.late};
    }
    constexpr Score operator*(const int lam) const noexcept {
        return Score{lam*early, lam*late};
    }
    constexpr Score& operator+=(const Score& rhs) noexcept {
        early += rhs.early;
        late  += rhs.late;
        return *this;
    }
    constexpr Score& operator-=(const Score& rhs) noexcept {
        early -= rhs.early;
        late  -= rhs.late;
        return *this;
    }
    constexpr Score& operator*=(const int lam) noexcept {
        early *= lam;
        late  *= lam;
        return *this;
    }
    constexpr bool operator==(const Score&) const = default;
    constexpr Score operator-() const noexcept { return {-early, -late}; }
};

} // namespace azalea
